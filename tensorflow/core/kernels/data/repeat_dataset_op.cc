/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include "tensorflow/core/framework/partial_tensor_shape.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/kernels/data/dataset.h"
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <fstream>
#include <string.h>
//using namespace std;
namespace tensorflow {

namespace {

// See documentation in ../ops/dataset_ops.cc for a high-level
// description of the following op.

class RepeatDatasetOp : public UnaryDatasetOpKernel {
 public:
  explicit RepeatDatasetOp(OpKernelConstruction* ctx)
      : UnaryDatasetOpKernel(ctx) {}

 protected:
  void MakeDataset(OpKernelContext* ctx, DatasetBase* input,
                   DatasetBase** output) override {
    // Create a new RepeatDatasetOp::Dataset, insert it in the step-local
    // container, and return it as the output.
	struct timespec t_start,t_end;
	clock_gettime(CLOCK_REALTIME,&t_start);
    int64 count;
    OP_REQUIRES_OK(ctx, ParseScalarArgument<int64>(ctx, "count", &count));
    *output = new Dataset(ctx, count, input);
	clock_gettime(CLOCK_REALTIME,&t_end);
	//cout<<"repeat_dataset_op,t_start,"<<t_start.tv_sec<<"."<<t_start.tv_nsec<<",t_end,"<<t_end.tv_sec<<"."<<t_end.tv_nsec<<endl;
  std::string op_name ="repeat_dataset_op,";
  std::string t_start_str = "t_start," + std::to_string(t_start.tv_sec) + "." + std::to_string(t_start.tv_nsec) + ",";
  std::string t_end_str = "t_end," + std::to_string(t_end.tv_sec) + "." + std::to_string(t_end.tv_nsec) + "\n";
  std::string result = op_name + t_start_str + t_end_str;
  std::ofstream file;
  file.open("TF_prepare.binary", std::ios::out | std::ios::app | std::ios::binary);
  file << result;
  }

 private:
  class Dataset : public GraphDatasetBase {
   public:
    Dataset(OpKernelContext* ctx, int64 count, const DatasetBase* input)
        : GraphDatasetBase(ctx), count_(count), input_(input) {
      input_->Ref();
    }

    ~Dataset() override { input_->Unref(); }

    std::unique_ptr<IteratorBase> MakeIterator(
        const string& prefix) const override {
      if (count_ < 0) {
        return std::unique_ptr<IteratorBase>(new ForeverIterator(
            {this, strings::StrCat(prefix, "::ForeverRepeat")}));
      } else if (count_ == 0) {
        return std::unique_ptr<IteratorBase>(new EmptyIterator(
            {this, strings::StrCat(prefix, "::EmptyRepeat")}));
      } else {
        return std::unique_ptr<IteratorBase>(new FiniteIterator(
            {this, strings::StrCat(prefix, "::FiniteRepeat")}));
      }
    }

    const DataTypeVector& output_dtypes() const override {
      return input_->output_dtypes();
    }
    const std::vector<PartialTensorShape>& output_shapes() const override {
      return input_->output_shapes();
    }

    string DebugString() override { return "RepeatDatasetOp::Dataset"; }

   protected:
    Status AsGraphDefInternal(OpKernelContext* ctx, DatasetGraphDefBuilder* b,
                              Node** output) const override {
      Node* input_graph_node = nullptr;
      TF_RETURN_IF_ERROR(b->AddParentDataset(ctx, input_, &input_graph_node));
      Node* count = nullptr;
      TF_RETURN_IF_ERROR(b->AddScalar(count_, &count));
      TF_RETURN_IF_ERROR(
          b->AddDataset(this, {input_graph_node, count}, output));
      return Status::OK();
    }

   private:
    class EmptyIterator : public DatasetIterator<Dataset> {
     public:
      explicit EmptyIterator(const Params& params)
          : DatasetIterator<Dataset>(params) {}
      Status GetNextInternal(IteratorContext* ctx,
                             std::vector<Tensor>* out_tensors,
                             bool* end_of_sequence) override {
        *end_of_sequence = true;
        return Status::OK();
      }

     protected:
      Status SaveInternal(IteratorStateWriter* writer) override {
        return Status::OK();
      }
      Status RestoreInternal(IteratorContext* ctx,
                             IteratorStateReader* reader) override {
        return Status::OK();
      }
    };

    class FiniteIterator : public DatasetIterator<Dataset> {
     public:
      explicit FiniteIterator(const Params& params)
          : DatasetIterator<Dataset>(params),
            i_(0),
            input_impl_(params.dataset->input_->MakeIterator(params.prefix)) {}

      Status GetNextInternal(IteratorContext* ctx,
                             std::vector<Tensor>* out_tensors,
                             bool* end_of_sequence) override {
		struct timespec t_start,t_end;
		clock_gettime(CLOCK_REALTIME,&t_start);
        mutex_lock l(mu_);  // TODO(mrry): Make locking less conservative.
        if (!input_impl_) {
          *end_of_sequence = true;
          return Status::OK();
        }
        while (i_ < dataset()->count_) {
          TF_RETURN_IF_ERROR(
              input_impl_->GetNext(ctx, out_tensors, end_of_sequence));
          if (!*end_of_sequence) {
            return Status::OK();
          }
          ++i_;
          input_impl_ = dataset()->input_->MakeIterator(prefix());
        }
        *end_of_sequence = true;
        input_impl_.reset();
		clock_gettime(CLOCK_REALTIME,&t_end);
		//cout<<"repeat_dataset_Finiteiterator,t_start,"<<t_start.tv_sec<<"."<<t_start.tv_nsec<<",t_end,"<<t_end.tv_sec<<"."<<t_end.tv_nsec<<endl;
    std::string op_name ="repeat_dataset_Finiteiterator,";
    std::string t_start_str = "t_start," + std::to_string(t_start.tv_sec) + "." + std::to_string(t_start.tv_nsec) + ",";
    std::string t_end_str = "t_end," + std::to_string(t_end.tv_sec) + "." + std::to_string(t_end.tv_nsec) + "\n";
    std::string result = op_name + t_start_str + t_end_str;
    std::ofstream file;
    file.open("TF_prepare.binary", std::ios::out | std::ios::app | std::ios::binary);
    file << result;
        return Status::OK();
      }

     protected:
      Status SaveInternal(IteratorStateWriter* writer) override {
        mutex_lock l(mu_);
        TF_RETURN_IF_ERROR(writer->WriteScalar(full_name("i"), i_));
        if (!input_impl_) {
          TF_RETURN_IF_ERROR(
              writer->WriteScalar(full_name("input_impl_empty"), ""));
        } else {
          TF_RETURN_IF_ERROR(SaveParent(writer, input_impl_));
        }
        return Status::OK();
      }

      Status RestoreInternal(IteratorContext* ctx,
                             IteratorStateReader* reader) override {
        mutex_lock l(mu_);
        TF_RETURN_IF_ERROR(reader->ReadScalar(full_name("i"), &i_));
        if (!reader->Contains(full_name("input_impl_empty"))) {
          TF_RETURN_IF_ERROR(RestoreParent(ctx, reader, input_impl_));
        } else {
          input_impl_.reset();
        }
        return Status::OK();
      }

     private:
      mutex mu_;
      int64 i_ GUARDED_BY(mu_);
      std::unique_ptr<IteratorBase> input_impl_ GUARDED_BY(mu_);
    };

    class ForeverIterator : public DatasetIterator<Dataset> {
     public:
      explicit ForeverIterator(const Params& params)
          : DatasetIterator<Dataset>(params), input_impl_(nullptr) {}

      Status GetNextInternal(IteratorContext* ctx,
                             std::vector<Tensor>* out_tensors,
                             bool* end_of_sequence) override {
		struct timespec t_start,t_end;
		clock_gettime(CLOCK_REALTIME,&t_start);
        mutex_lock l(mu_);  // TODO(mrry): Make locking less conservative.
        do {
          bool first_call = false;
          if (!input_impl_) {
            first_call = true;
            input_impl_ = dataset()->input_->MakeIterator(prefix());
          }
          TF_RETURN_IF_ERROR(
              input_impl_->GetNext(ctx, out_tensors, end_of_sequence));
          if (!*end_of_sequence) {
            return Status::OK();
          } else {
            input_impl_.reset();
            if (first_call) {
              // If the first call to GetNext() fails because the end
              // of sequence has been reached, we terminate the
              // iteration immediately. (Otherwise, this iterator
              // would loop infinitely and never produce a value.)
			  clock_gettime(CLOCK_REALTIME,&t_end);
			  //cout<<"repeat_dataset_Foreveriterator,t_start,"<<t_start.tv_sec<<"."<<t_start.tv_nsec<<",t_end,"<<t_end.tv_sec<<"."<<t_end.tv_nsec<<endl;
        std::string op_name ="repeat_dataset_Foreveriterator,";
        std::string t_start_str = "t_start," + std::to_string(t_start.tv_sec) + "." + std::to_string(t_start.tv_nsec) + ",";
        std::string t_end_str = "t_end," + std::to_string(t_end.tv_sec) + "." + std::to_string(t_end.tv_nsec) + "\n";
        std::string result = op_name + t_start_str + t_end_str;
        std::ofstream file;
        file.open("TF_prepare.binary", std::ios::out | std::ios::app | std::ios::binary);
        file << result;
              return Status::OK();
            }
          }
        } while (true);
      }

     protected:
      Status SaveInternal(IteratorStateWriter* writer) override {
        mutex_lock l(mu_);
        if (input_impl_)
          TF_RETURN_IF_ERROR(SaveParent(writer, input_impl_));
        else
          TF_RETURN_IF_ERROR(
              writer->WriteScalar(full_name("uninitialized"), ""));
        return Status::OK();
      }

      Status RestoreInternal(IteratorContext* ctx,
                             IteratorStateReader* reader) override {
        mutex_lock l(mu_);
        if (reader->Contains(full_name("uninitialized"))) {
          input_impl_.reset();
        } else {
          input_impl_ = dataset()->input_->MakeIterator(prefix());
          TF_RETURN_IF_ERROR(RestoreParent(ctx, reader, input_impl_));
        }
        return Status::OK();
      }

     private:
      mutex mu_;
      std::unique_ptr<IteratorBase> input_impl_ GUARDED_BY(mu_);
    };

    const int64 count_;
    const DatasetBase* const input_;
  };
};

REGISTER_KERNEL_BUILDER(Name("RepeatDataset").Device(DEVICE_CPU),
                        RepeatDatasetOp);

}  // namespace

}  // namespace tensorflow
